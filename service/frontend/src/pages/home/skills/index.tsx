import React, { useEffect } from "react";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";

import { fetcher } from "src/fetcher";

import "./index.css";

type skill = {
    name: string;
    dates: number[];
    occurences: number[];
    total_occurences: number;
};

type state = skill[];

const endpoint = "api/skills";

const selector = (o: any): skill[] => o;

function Occurences() {
    const initial: state = [];

    const [setUrl, data, error, loading, restart] = fetcher(
        endpoint,
        "GET",
        initial,
        selector
    );

    if (!loading && !error) {
        console.log(data);
    }

    useEffect(() => {
        if (error) restart(10000);
    }, [error]);

    return (
        <>
            <Heading tag="h4" level={3}>
                Occurences
            </Heading>
            <SubHeading tag="p" level={3}>
                occurence over time
            </SubHeading>
            <Text.Component tag="p">
                This graph depicts skill popularity.
            </Text.Component>
        </>
    );
}

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
                <Occurences />
            </Container.Component>
        </Section.Component>
    );
}
