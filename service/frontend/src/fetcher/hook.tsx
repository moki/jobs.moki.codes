import { useState, useEffect, useReducer } from "preact/hooks";

import {
    reducer,
    FETCH_INIT,
    FETCH_SUCC,
    FETCH_FAIL,
    State,
    Action,
} from "./reducer";

export const HTTPMethods = [
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "CONNECT",
    "OPTIONS",
    "TRACE",
    "PATCH",
] as const;

export type HTTPMethod = typeof HTTPMethods[number];

export type IntoString = { toString: () => string };

export type Endpoint = IntoString | string;

const oks: { [key: number]: boolean } = {
    200: true,
    201: true,
    202: true,
    203: true,
    204: true,
    205: true,
    206: true,
    207: true,
    208: true,
    226: true,
};

function statusok(status: number): boolean {
    return oks[status] ?? false;
}

export function hook<S, E extends Endpoint>(
    endpoint: E,
    method: HTTPMethod,
    initial: S,
    selector: <O>(_: O) => S
) {
    const ep = typeof endpoint === "string" ? endpoint : endpoint.toString();

    const [{ data, loading, error }, dispatch] = useReducer<
        State<S>,
        Action<S>
    >(reducer, {
        data: initial,
        error: false,
        loading: true,
    });

    const [url, setUrl] = useState(ep);

    const [trigger, setTrigger] = useState(false);

    const restart = (delay: number) => {
        setTimeout(function () {
            setTrigger(!trigger);
        }, delay);
    };

    useEffect(() => {
        let mounted = true;

        const request = new XMLHttpRequest();

        request.addEventListener("load", (e) => {
            if (!mounted) return;

            const { status } = request;

            if (!statusok(status)) {
                dispatch({ type: FETCH_FAIL });

                return;
            }

            try {
                let parsed = JSON.parse(request.response);

                let data = selector(parsed);

                dispatch({ type: FETCH_SUCC, payload: data });
            } catch {
                dispatch({ type: FETCH_FAIL });
            }
        });

        request.addEventListener("error", (e) => {
            if (!mounted) return;

            dispatch({ type: FETCH_FAIL });
        });

        request.open(method, ep);

        request.send();

        dispatch({ type: FETCH_INIT });

        return () => (mounted = false);
    }, [url, trigger]);

    return [setUrl, data, error, loading, restart] as const;
}
