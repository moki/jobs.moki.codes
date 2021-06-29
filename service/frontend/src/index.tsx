import React from "react";
import ReactDOM from "react-dom";

import "lib/reset.css";

import { Theme } from "src/components/theme";
import { Layout } from "src/components/layout";
import { Header } from "src/components/header";
import { Banner } from "src/components/banner";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Main } from "src/components/main";
import { Footer } from "src/components/footer";

import { Home } from "src/pages/home";

import "./index.css";

export type CustomWindow = {
    debug: boolean;
} & Window &
    typeof globalThis;

function App() {
    let { classes } = Theme();

    classes += " " + Layout.classes;

    return (
        <div className={classes}>
            <Header>
                <Banner>
                    <Heading tag="h1" level={1}>
                        Russian IT Hiring Trends
                    </Heading>
                    <SubHeading tag="h2" level={1}>
                        ongoing data study into russia's job market
                    </SubHeading>
                </Banner>
            </Header>
            <Main>
                <Home />
            </Main>
            <Footer />
        </div>
    );
}

ReactDOM.render(<App />, document.querySelector("#app"));
