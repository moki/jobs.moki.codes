import React from "react";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";

export function Status() {
    return (
        <Section.Component>
            <Container.Component>
                <Heading tag="h3" level={2}>
                    Status
                </Heading>
                <SubHeading tag="p" level={2}>
                    tracking progress
                </SubHeading>
                <Text.Component tag="p">
                    Line chart visualization of the skills jobs include.
                </Text.Component>
            </Container.Component>
        </Section.Component>
    );
}
