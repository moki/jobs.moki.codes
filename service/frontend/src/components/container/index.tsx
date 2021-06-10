import React, { HTMLProps } from "react";

import "./index.css";

const classes = "container";

function Component({
    children,
    className,
    ...rest
}: HTMLProps<HTMLDivElement>) {
    const cs = classes + (className ? " " + className : "");

    return (
        <div {...rest} className={cs}>
            {children}
        </div>
    );
}

export const Container = { Component, classes };
