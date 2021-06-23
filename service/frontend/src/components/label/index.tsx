import React, { HTMLProps } from "react";

import "./index.css";

export type LabelProps = {
    text: string;
    id: string;
} & HTMLProps<HTMLLabelElement>;

export function Label({ text, id }: LabelProps) {
    const classes = "label";

    return (
        <label htmlFor={id} className={classes}>
            {text}
        </label>
    );
}
