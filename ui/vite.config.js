import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";

export default defineConfig({
  base: "./",
  plugins: [
    vue(),
    {
      name: "ultralight-indexhtml",
      enforce: "post",
      transformIndexHtml(html) {
        const withoutModuleScripts = html.replace(
          /<script[^>]*type="module"[^>]*>\s*<\/script>/g,
          "",
        );

        const withoutEntryModuleScript = withoutModuleScripts.replace(
          /<script[^>]*type="module"[^>]*src="[^"]*src\/main\.js"[^>]*><\/script>/g,
          "",
        );

        if (withoutEntryModuleScript.includes("./assets/app.js")) {
          return withoutEntryModuleScript;
        }

        return withoutEntryModuleScript.replace(
          "</body>",
          '  <script src="./assets/app.js"></script>\n  </body>',
        );
      },
    },
  ],
  build: {
    outDir: "dist",
    emptyOutDir: true,
    target: "es2015",
    cssCodeSplit: false,
    modulePreload: false,
    rollupOptions: {
      input: "index.html",
      output: {
        format: "iife",
        inlineDynamicImports: true,
        entryFileNames: "assets/app.js",
        chunkFileNames: "assets/app.js",
        assetFileNames(assetInfo) {
          if (assetInfo.name && assetInfo.name.endsWith(".css")) {
            return "assets/style.css";
          }
          return "assets/[name][extname]";
        },
      },
    },
  },
  server: {
    port: 5173,
    strictPort: true,
  },
});
