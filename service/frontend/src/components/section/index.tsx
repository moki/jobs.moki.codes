import React, { ReactNode, HTMLProps } from "react";

import "./index.css";

const classes = "section";

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

export const Section = { Component, classes };
