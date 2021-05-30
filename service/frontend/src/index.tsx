import { render, h } from "preact";

import "lib/reset.css";

import { Theme } from "src/components/theme";
import { Layout } from "src/components/layout";
import { Header } from "src/components/header";
import { Main } from "src/components/main";
import { Footer } from "src/components/footer";

import { Home } from "src/pages/home";

import "./index.css";

function App() {
    let { classes } = Theme();

    classes += " " + Layout.classes;

    return (
        <div class={classes}>
            <Header />
            <Main>
                <Home />
            </Main>
            <Footer />
        </div>
    );
}

render(<App />, document.body);
