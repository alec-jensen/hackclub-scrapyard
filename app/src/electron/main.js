import * as url from "url";
import { app, BrowserWindow } from "electron";

app.whenReady().then(async () => {
  const win = new BrowserWindow({
    title: "Main window",
    webPreferences: {
      preload: url.fileURLToPath(new URL("preload.mjs", import.meta.url)),
    },
  });

  // You can use `process.env.VITE_DEV_SERVER_URL` when the vite command is called `serve`
  if (process.env.VITE_DEV_SERVER_URL) {
    await win.loadURL(process.env.VITE_DEV_SERVER_URL);
    win.webContents.openDevTools();
  } else {
    // Load your file
    await win.loadFile("dist/index.html");
  }
});
