import { h } from "preact";

import { ChildrenProp } from "src/components/component";

export type MainProps = ChildrenProp;

export function Main({ children }: MainProps) {
    return <main class="layout_hg__main">{children}</main>;
}
