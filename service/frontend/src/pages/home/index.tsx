import { h, Fragment } from "preact";

import { Container } from "src/components/container";
import { Section } from "src/components/section";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";

import "./index.css";

export function Home() {
    return (
        <Fragment>
            <Section.Component>
                <Container.Component class="hey">
                    <Heading tag="h3" level={2}>
                        Skills
                    </Heading>
                    <SubHeading tag="p" level={2}>
                        occurences over time
                    </SubHeading>
                </Container.Component>
            </Section.Component>
        </Fragment>
    );
}
