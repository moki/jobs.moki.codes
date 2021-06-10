import React, { HTMLProps } from "react";

const classes = "layout_hg__main";

export function Main({ children, ...rest }: HTMLProps<HTMLElement>) {
    return (
        <main {...rest} className={classes}>
            {children}
        </main>
    );
}
