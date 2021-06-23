import { useEffect, RefObject } from "react";

export function useOutsideEventAwareness(
    ref: RefObject<any>,
    event: (keyof DocumentEventMap)[],
    handler: Function
) {
    useEffect(() => {
        const handleEvent = (e: Event) => {
            ref &&
                ref.current &&
                ref.current.contains &&
                !ref.current.contains(e.target) &&
                handler();
        };

        event.map((e) => document.addEventListener(e, handleEvent));

        return () => {
            event.map((e) => document.removeEventListener(e, handleEvent));
        };
    }, [ref]);
}
