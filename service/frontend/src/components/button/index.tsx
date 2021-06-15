import React, { HTMLProps } from "react";

import "./index.css";

export const ButtonTypes = ["outlined"] as const;

export type ButtonType = typeof ButtonTypes[number];

export type ButtonProps = {
    type: ButtonType;
} & HTMLProps<HTMLButtonElement>;

export function Button({ type, children, ...rest }: ButtonProps) {
    const classes = {
        base: "button",
        outlined: "button_type_outlined",
    };

    const classNames = classes.base + " " + classes[type];

    return (
        <button {...rest} className={classNames}>
            {children}
        </button>
    );
}
