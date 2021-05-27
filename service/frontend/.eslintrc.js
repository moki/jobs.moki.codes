module.exports = {
    settings: {
        pragma: "h",
        pragmaFrag: "Fragment"
    },
    parser: "@typescript-eslint/parser",
    parserOptions: {
        ecmaFeatures: {
            jsx: true
        },
        project: "tsconfig.json",
        tsconfigRootDir: __dirname
    },
    extends: [
        "eslint:recommended",
        "plugin:@typescript-eslint/eslint-recommended",
        "plugin:@typescript-eslint/recommended"
        "prettier",
        "prettier/babel",
    ],
    plugins: ["babel", "react", "@typescript-eslint"],
    env: {
        es2020: true,
        browser: true,
    },
    rules: {
        "react/jsx-uses-react": "error",
    }
};
