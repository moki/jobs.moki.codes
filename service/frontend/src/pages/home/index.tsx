import { h, Fragment } from "preact";

import { Introduction } from "src/pages/home/introduction";
import { Skills } from "src/pages/home/skills";
import { Status } from "src/pages/home/status";

import "./index.css";

export function Home() {
    return (
        <Fragment>
            <Introduction />
            <Skills />
            <Status />
        </Fragment>
    );
}
