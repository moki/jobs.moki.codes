import { Fragment, createElement, render, h } from "preact";

import "./index.css";
import "lib/reset.css";

function repeat_it<T>(token: T) {
    const it = {
        next: function (): { value: T; done: boolean } {
            return { value: token, done: false };
        },
        take: function (this, n: number): Array<T> {
            return [...new Array(n)].map((e) => this.next().value);
        },
    };

    return { [Symbol.iterator]: () => ({ ...it }) };
}

function App() {
    let pog_it = repeat_it("pog chompus")[Symbol.iterator]();

    let fives = (
        <Fragment>
            {pog_it.take(10).map((e, i) => (
                <li>
                    {e} {i}
                </li>
            ))}
        </Fragment>
    );

    return (
        <div class="pog-div">
            pog champs:
            <ul>{fives}</ul>
        </div>
    );
}

render(<App />, document.body);
