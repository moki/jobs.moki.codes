import React, { HTMLProps } from "react";

import "./index.css";

type LabelProps = {
    text: string;
} & HTMLProps<HTMLLabelElement>;

export function Label({ id, text }: LabelProps) {
    const classes = "label";

    return (
        <label htmlFor={id} className={classes}>
            {text}
        </label>
    );
}
