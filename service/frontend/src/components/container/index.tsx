import { h } from "preact";

import "./index.css";

import { ChildrenProp, HTMLAttributes } from "src/components/component";

const classes = "container";

export type ContainerProps = ChildrenProp & HTMLAttributes;

function Component({ children, ...rest }: ContainerProps) {
    const cs = classes + (rest.class ? " " + rest.class : "");

    return (
        <div {...rest} class={cs}>
            {children}
        </div>
    );
}

export const Container = { Component, classes };
