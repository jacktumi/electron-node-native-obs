// Modules to control application life and create native browser window
const { app, BrowserWindow } = require("electron");
const path = require("node:path");
const fs = require("fs");

function createWindow() {
    // Create the browser window.
    const mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: path.join(__dirname, "preload.js"),
        },
    });

    // and load the index.html of the app.
    mainWindow.loadFile("index.html");

    const addon = require("./node_native/build/Release/native_module");
    console.log(`INFO: ${addon.getVersion()}`);

    // Load settings from settings.json
    const settingsPath = path.join(__dirname, "settings.json");
    let obsConfig;
    try {
        const settingsContent = fs.readFileSync(settingsPath, "utf-8");
        obsConfig = JSON.parse(settingsContent);
        console.log("INFO: Settings loaded successfully");
        console.log(obsConfig);
    } catch (error) {
        console.error("Failed to load settings:", error);
        return;
    }

    const initStreamer = addon.initStreamer(
        obsConfig.obs_data_path,
        obsConfig.obs_module_bin_path,
        obsConfig.obs_module_data_path
    );
    console.log(`INFO: obs streamer initialization results: ${initStreamer}`);
    const initTwitch = addon.setupStreaming(
        obsConfig.twitch_rtmp,
        obsConfig.twitch_key
    );
    console.log(`INFO: Twitch streaming setup results: ${initTwitch}`);
    const streamStatus = addon.startStreaming();
    console.log(`INFO: Streaming start status: ${streamStatus}`);
    // Wait for 30 seconds before stopping the stream
    setTimeout(() => {
        const streamStatus = addon.stopStreaming();
        console.log(`INFO: Streaming stop status: ${streamStatus}`);
    }, 30000);
    
    // Open the DevTools.
    // mainWindow.webContents.openDevTools()
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
    createWindow();

    app.on("activate", function () {
        // On macOS it's common to re-create a window in the app when the
        // dock icon is clicked and there are no other windows open.
        if (BrowserWindow.getAllWindows().length === 0) createWindow();
    });
});

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on("window-all-closed", function () {
    if (process.platform !== "darwin") app.quit();
});

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
