import { useState } from "react";

import "./index.css";

const LIGHT = false;
const DARK = true;

const LIGHT_CLASS = "light";
const DARK_CLASS = "dark";

const classes = "theme theme_typography";

export function Theme(initialColor: boolean = LIGHT) {
    const initialThemeColor =
        window.localStorage.getItem("theme-color") !== null
            ? window.localStorage.getItem("theme-color") === "true"
            : LIGHT;

    const [color, setColor] = useState(initialThemeColor);

    let cs = classes + ` theme_color_${color ? DARK_CLASS : LIGHT_CLASS}`;

    const toggleColor = (e: any) => {
        let s = color ? DARK_CLASS : LIGHT_CLASS;
        let t = color ? LIGHT_CLASS : DARK_CLASS;

        cs.replace(s, t);

        setColor(!color);

        window.localStorage.setItem("theme-color", "" + !color);
    };

    return {
        color,
        toggleColor,
        classes: cs,
    };
}
