import { defineConfig } from "astro/config";
import electron from "astro-electron";

export default defineConfig({
    integrations: [electron({
        main: {
            entry: "src/electron/main.js", // Path to your Electron main file
            vite: {}, // Vite-specific configurations (by default we use the same config as your Astro project)
        }
    }
    )],
});