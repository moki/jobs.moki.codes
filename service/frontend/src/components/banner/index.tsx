import React, { HTMLProps } from "react";

import { Section } from "src/components/section";
import { Container } from "src/components/container";

const classes = "banner";

export function Banner({ children, ...rest }: HTMLProps<HTMLDivElement>) {
    return (
        <Section.Component>
            <Container.Component {...rest} className={classes}>
                {children}
            </Container.Component>
        </Section.Component>
    );
}
