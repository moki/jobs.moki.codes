import React, { HTMLProps } from "react";

import { Section } from "src/components/section";

import "./index.css";

const classes = "layout_hg__footer footer";

export function Footer({ children, ...rest }: HTMLProps<HTMLDivElement>) {
    return (
        <footer {...rest} className={classes}>
            <Section.Component>{children}</Section.Component>
        </footer>
    );
}
