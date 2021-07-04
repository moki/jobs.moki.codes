use std::fmt;

use serde::Serialize;

use crate::Result;

#[derive(Debug, Clone, Copy, Serialize)]
pub struct Quartiles {
    pub lower_fence: f64,
    pub first: f64,
    pub median: f64,
    pub third: f64,
    pub upper_fence: f64,
}

impl fmt::Display for Quartiles {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "lower_fence: {},\nfirst: {},\nmedian: {},\nthird: {},\nupper_fence: {},\n",
            self.lower_fence, self.first, self.median, self.third, self.upper_fence
        )
    }
}

pub fn calculate_quartiles(sequence: &[u64]) -> Result<Quartiles, &'static str> {
    let seq_len = sequence.len();

    if seq_len == 0 {
        return Err("give non-empty slice");
    }

    let even = seq_len % 2 == 0;

    let median_index = calculate_median_index(seq_len, even)?;

    let left = &sequence[..(if even { median_index + 1 } else { median_index })];

    let first = calculate_median(left)?;

    let right = &sequence[median_index + 1..];

    let third = calculate_median(right)?;

    let median = calculate_median(sequence)?;

    let iqr = third - first;

    let lower_fence = first - iqr * 1.5_f64;

    let upper_fence = third + iqr * 1.5_f64;

    Ok(Quartiles {
        lower_fence,
        first,
        median,
        third,
        upper_fence,
    })
}

fn calculate_median(sequence: &[u64]) -> Result<f64, &'static str> {
    if sequence.len() == 0 {
        return Err("give non-empty slice");
    }

    let even = sequence.len() % 2 == 0;

    let median_index = match calculate_median_index(sequence.len(), even) {
        Ok(median_index) => median_index,
        _ => return Err("give non-empty slice"),
    };

    if sequence.len() == 1 {
        return Ok(sequence[median_index] as f64);
    }

    if even {
        Ok((sequence[median_index] as f64 + sequence[median_index + 1] as f64) / 2 as f64)
    } else {
        Ok(sequence[median_index] as f64)
    }
}

fn calculate_median_index(sequence_length: usize, even: bool) -> Result<usize, &'static str> {
    if sequence_length == 0 {
        return Err("give non-empty slice");
    }

    if sequence_length == 1 {
        return Ok(0 as usize);
    }

    if even {
        Ok(sequence_length / 2 - 1)
    } else {
        Ok((sequence_length + 1) / 2 - 1)
    }
}

#[cfg(test)]
mod tests {
    pub use super::*;

    #[test]
    fn calculate_median_index_odd_test() {
        let xs: Vec<u64> = vec![6, 7, 15, 36, 39, 40, 41, 42, 43, 47, 49];

        let left = calculate_median_index(xs.len(), xs.len() % 2 == 0);
        let right = 5;

        assert_eq!(left.is_err(), false);
        assert_eq!(left.unwrap(), right);
    }

    #[test]
    fn calculate_median_index_even_test() {
        let xs: Vec<u64> = vec![6, 7, 15, 36, 39, 40, 41, 42, 43, 47];

        let left = calculate_median_index(xs.len(), xs.len() % 2 == 0);
        let right = 4;

        assert_eq!(left.is_err(), false);
        assert_eq!(left.unwrap(), right);
    }

    #[test]
    fn calculate_median_even_test() {
        let xs: Vec<u64> = vec![6, 7, 15, 36, 39, 40, 41, 42, 43, 47];

        let left = calculate_median(xs.as_slice());
        let right = 39.5_f64;

        assert_eq!(left.is_err(), false);

        assert_eq!(left.unwrap(), right);
    }

    #[test]
    fn calculate_median_odd_test() {
        let xs: Vec<u64> = vec![6, 7, 15, 36, 39, 40, 41, 42, 43, 47, 49];

        let left = calculate_median(xs.as_slice());
        let right = 40_f64;

        assert_eq!(left.is_err(), false);
        assert_eq!(left.unwrap(), right);
    }

    #[test]
    fn calculate_quartiles_even_test() {
        // test case 1
        let xs: Vec<u64> = vec![6, 7, 15, 36, 39, 40, 41, 42, 43, 47];

        let left = calculate_quartiles(xs.as_slice());
        let right = Quartiles {
            lower_fence: -25.5,
            first: 15.0,
            median: 39.5,
            third: 42.0,
            upper_fence: 82.5,
        };

        assert_eq!(left.is_err(), false);

        let left = left.unwrap();

        println!("left:\n{}", left);
        println!("right:\n{}", right);

        assert_eq!(left.lower_fence, right.lower_fence);
        assert_eq!(left.first, right.first);
        assert_eq!(left.median, right.median);
        assert_eq!(left.third, right.third);
        assert_eq!(left.upper_fence, right.upper_fence);

        // test case 2

        let ys: Vec<u64> = vec![7, 15, 36, 39, 40, 41];

        let left = calculate_quartiles(ys.as_slice());
        let right = Quartiles {
            lower_fence: -22.5,
            first: 15.0,
            median: 37.5,
            third: 40.0,
            upper_fence: 77.5,
        };

        assert_eq!(left.is_err(), false);

        let left = left.unwrap();

        println!("left:\n{}", left);
        println!("right:\n{}", right);

        assert_eq!(left.lower_fence, right.lower_fence);
        assert_eq!(left.first, right.first);
        assert_eq!(left.median, right.median);
        assert_eq!(left.third, right.third);
        assert_eq!(left.upper_fence, right.upper_fence);
    }

    #[test]
    fn calculate_quartiles_odd_test() {
        let xs: Vec<u64> = vec![6, 7, 15, 36, 39, 40, 41, 42, 43, 47, 49];

        let left = calculate_quartiles(xs.as_slice());
        let right = Quartiles {
            lower_fence: -27.0,
            first: 15.0,
            median: 40.0,
            third: 43.0,
            upper_fence: 85.0,
        };

        assert_eq!(left.is_err(), false);

        let left = left.unwrap();

        println!("left:\n{}", left);
        println!("right:\n{}", right);

        assert_eq!(left.lower_fence, right.lower_fence);
        assert_eq!(left.first, right.first);
        assert_eq!(left.median, right.median);
        assert_eq!(left.third, right.third);
        assert_eq!(left.upper_fence, right.upper_fence);
    }
}
