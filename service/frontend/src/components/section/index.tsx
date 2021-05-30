import { h } from "preact";

import "./index.css";

import { ChildrenProp, HTMLAttributes } from "src/components/component";

const classes = "section";

export type SectionProps = ChildrenProp & HTMLAttributes;

function Component({ children, ...rest }: SectionProps) {
    const cs = classes + (rest.class ? " " + rest.class : "");

    return (
        <div {...rest} class={cs}>
            {children}
        </div>
    );
}

export const Section = { Component, classes };
