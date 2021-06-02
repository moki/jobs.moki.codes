import { h } from "preact";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";

export function Introduction() {
    return (
        <Section.Component>
            <Container.Component>
                <Heading tag="h3" level={2}>
                    Introduction
                </Heading>
                <SubHeading tag="p" level={2}>
                    expectations and goals
                </SubHeading>
                <Text.Component tag="p">
                    This project aims to paint, up to date picture of the job
                    market available for the russian speaking developers.
                </Text.Component>
                <Text.Component tag="p">
                    Data is aggregated daily and visualized in different forms.
                </Text.Component>
                <Text.Component tag="p">
                    Please refer to the "Status" section for the list of
                    implemented features and roadmap of the ones to come.
                </Text.Component>
            </Container.Component>
        </Section.Component>
    );
}
