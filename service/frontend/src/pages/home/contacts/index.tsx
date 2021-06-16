import React, { HTMLProps } from "react";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";

import "./index.css";

type LinkData = { label: string; url: string };

type LinksProps = {
    links: LinkData[];
} & HTMLProps<HTMLUListElement>;

function Links({ links }: LinksProps) {
    return (
        <ul className="link-items">
            {links.map(({ label, url }: LinkData, i: number) => (
                <li key={i} className="link-item">
                    <a
                        className="link"
                        href={url}
                        target="_blank"
                        rel="noopener"
                    >
                        {label}
                    </a>
                </li>
            ))}
        </ul>
    );
}

export function Contacts() {
    const ls = [
        {
            label: "github",
            url: "https://github.com/moki/jobs.moki.codes",
        },
        {
            label: "telegram",
            url: "https://t.me/mokicodes",
        },
        {
            label: "twitter",
            url: "https://twitter.com/mokicodes",
        },
        {
            label: "email",
            url: "mailto:morozov.kirill.moki@gmail.com",
        },
    ];

    return (
        <Section.Component>
            <Container.Component>
                <Heading tag="h3" level={2}>
                    Contacts
                </Heading>
                <SubHeading tag="p" level={2}>
                    socials
                </SubHeading>
                <Text.Component tag="p">
                    Let me know if you have any suggestions/critique/found bugs.
                </Text.Component>
                <Links links={ls} />
            </Container.Component>
        </Section.Component>
    );
}
