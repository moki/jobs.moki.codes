import { useState, useEffect } from "preact/hooks";

function getWidth() {
    return (
        window.innerWidth ||
        document.documentElement.clientWidth ||
        document.body.clientWidth
    );
}

function getHeight() {
    return (
        window.innerHeight ||
        document.documentElement.clientHeight ||
        document.body.clientHeight
    );
}

export function hook() {
    let [width, setWidth] = useState(getWidth());
    let [height, setHeight] = useState(getHeight());

    useEffect(() => {
        let timeout_id: number;

        function listener() {
            clearTimeout(timeout_id);

            timeout_id = setTimeout(() => {
                setWidth(getWidth());
                setHeight(getHeight());
            }, 100);
        }

        window.addEventListener("resize", listener);

        return () => {
            window.removeEventListener("resize", listener);
        };
    }, []);

    return [width, height];
}
