import React, { useEffect } from "react";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";

import { Distribution } from "./distribution";

import "./index.css";

export function Salaries() {
    return (
        <Section.Component>
            <Container.Component>
                <Heading tag="h3" level={2}>
                    Salaries
                </Heading>
                <SubHeading tag="p" level={2}>
                    job compensations
                </SubHeading>
                <Text.Component tag="p">
                    Exploring job compensations provided by the market.
                </Text.Component>
                <Distribution />
            </Container.Component>
        </Section.Component>
    );
}
