#!/usr/bin/env node

const systemInfo = require("./systemInfo");
const promiseMap = require("./benchmarks/promiseMap");
const chalk = require("chalk");
const path = require("path");
const fs = require("fs");
const { fork } = require("child_process");
const benchReport = require("./benchmarks/benchReport");
const spinner = require("./benchmarks/spinner");

function listBenchFiles() {
  return new Promise((resolve, reject) => {
    const folder = path.resolve(path.join(__dirname, "../benchmarks"));
    fs.readdir(folder, (err, files) => {
      if (err) {
        reject(err);
      } else {
        resolve(
          files
            .filter((file) => file.endsWith(".bench.js"))
            .sort()
            .map((file) => {
              return path.join(folder, file);
            }),
        );
      }
    });
  });
}

function runBenchFileAsync(benchFile) {
  return new Promise((resolve, reject) => {
    let hasErrors = false;
    const forked = fork(benchFile, [benchReport.colorFlags], {
      env: { NODE_BENCHMARK_FORKED: 1 },
    });

    forked.on("error", reject);

    forked.on("message", (message) => {
      if (message && message.type === "suiteReport") {
        spinner.clear();
        if (message.hasErrors) {
          hasErrors = true;
        }
        benchReport.printSuiteName(message.name);
        benchReport.printSuiteReport(message);
      }
    });

    forked.on("exit", (code) => {
      if (code !== 0) {
        const error = new Error(`"${benchFile}" failed.`);
        error.stack = `Error: ${error.message}`;
        reject(error);
      } else {
        resolve(!hasErrors);
      }
    });
  });
}

async function benchmarks() {
  systemInfo.print();
  spinner.start();
  let hasErrors = false;
  try {
    const benchFiles = await listBenchFiles();
    console.log(
      chalk.green("*"),
      chalk.greenBright("running"),
      chalk.cyanBright(benchFiles.length),
      chalk.greenBright("files..."),
      "\n",
    );
    await promiseMap(
      benchFiles,
      async (benchFile) => {
        if (!(await runBenchFileAsync(benchFile))) {
          hasErrors = true;
        }
      },
      Math.max(1, systemInfo.physicalCpuCount - 1),
    );
    if (hasErrors) {
      const error = new Error("Benchmarks failed");
      error.stack = `Error: ${error.message}`;
      throw error;
    }
  } finally {
    spinner.stop();
  }
}

function run() {
  const completedMessage = `\n${chalk.green("*")} ${chalk.greenBright("completed")}`;
  console.time(completedMessage);

  return benchmarks()
    .then(() => {
      spinner.clear();
      console.timeEnd(completedMessage);
      console.log();
      return true;
    })
    .catch((error) => {
      spinner.clear();
      console.error(chalk.red("*"), chalk.redBright("FAIL"), chalk.red("-"), error, "\n");
      if (!process.exitCode) {
        process.exitCode = 1;
      }
      return false;
    });
}

module.exports = benchmarks;

benchmarks.run = run;

if (require.main === module) {
  run().catch((err) => {
    // eslint-disable-next-line no-console
    console.error(err);
  });
}
