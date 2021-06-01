import { h } from "preact";

import "./index.css";

import {
    TagProp,
    ChildrenProp,
    HTMLAttributes,
} from "src/components/component";

const classes = "text";

export type TextProp = TagProp & ChildrenProp & HTMLAttributes;

function Component({ tag, children, ...rest }: TextProp) {
    const cs = classes + (rest.class ? " " + rest.class : "");

    return h(tag, { ...rest, class: cs }, children);
}

export const Text = {
    Component,
    classes,
};
