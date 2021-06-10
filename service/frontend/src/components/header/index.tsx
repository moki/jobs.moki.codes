import React, { HTMLProps } from "react";

import "./index.css";

const classes = "layout_hg__header header";

export function Header({ children, ...rest }: HTMLProps<HTMLElement>) {
    return (
        <header {...rest} className={classes}>
            {children}
        </header>
    );
}
