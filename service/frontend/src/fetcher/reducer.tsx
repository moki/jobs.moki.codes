export const FETCH_INIT = 0;
export const FETCH_SUCC = 1;
export const FETCH_FAIL = 2;

export const Actions = [FETCH_INIT, FETCH_SUCC, FETCH_FAIL] as const;

export type Action<D> = {
    type: typeof Actions[number];
    payload?: D;
};

export type State<D> = {
    data: D;
    error: boolean;
    loading: boolean;
};

// TODO: think of the way to avoid Action<any>
//       and pass actual payload as a generic parameter
//       while still keeping the rest of the Action<D> properties.
//
//       passing actual generic type argument breaks
//       typesystem's ability to recognize <type> property.
export function reducer<S, A extends Action<any>>(state: S, action: A) {
    switch (action.type) {
        case FETCH_INIT:
            return { ...state, loading: true, error: false };
        case FETCH_SUCC:
            return {
                ...state,
                data: action.payload,
                loading: false,
                error: false,
            };
        case FETCH_FAIL:
            return { ...state, loading: false, error: true };
    }
}
