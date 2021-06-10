import React, { HTMLProps } from "react";

import "./index.css";

import { TagProp } from "src/components/component";

export type TextProp = TagProp & HTMLProps<HTMLElement>;

const classes = "text";

function Component({ tag, className, children, ...rest }: TextProp) {
    const cs = classes + (className ? " " + className : "");

    return React.createElement(tag, { ...rest, className: cs }, children);
}

export const Text = {
    Component,
    classes,
};
