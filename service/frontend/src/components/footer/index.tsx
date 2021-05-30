import { h } from "preact";

import { Section } from "src/components/section";

import "./index.css";

import { ChildrenProp } from "src/components/component";

export type FooterProps = ChildrenProp;

export function Footer({ children }: FooterProps) {
    return (
        <footer class="layout_hg__footer footer">
            <Section.Component>{children}</Section.Component>
        </footer>
    );
}
