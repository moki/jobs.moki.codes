import { h } from "preact";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";

import "./index.css";

function Banner() {
    return (
        <Section.Component>
            <Container.Component class="banner">
                <Heading tag="h1" level={1}>
                    Russian IT Hiring Trends
                </Heading>
                <SubHeading tag="h2" level={1}>
                    ongoing data study into russia's job market
                </SubHeading>
            </Container.Component>
        </Section.Component>
    );
}

export function Header() {
    const cs = "layout_hg__header header";

    return (
        <header class={cs}>
            <Banner />
        </header>
    );
}
