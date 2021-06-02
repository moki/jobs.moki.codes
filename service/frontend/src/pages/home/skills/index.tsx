import { h } from "preact";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";

import "./index.css";

export function Skills() {
    return (
        <Section.Component>
            <Container.Component>
                <Heading tag="h3" level={2}>
                    Skills
                </Heading>
                <SubHeading tag="p" level={2}>
                    job requirements
                </SubHeading>
                <Text.Component tag="p">
                    Exploring requirements trends in the job postings.
                </Text.Component>
            </Container.Component>
        </Section.Component>
    );
}
