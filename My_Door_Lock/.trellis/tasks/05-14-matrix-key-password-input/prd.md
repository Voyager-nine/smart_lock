# Matrix Key Password Input

## Requirements

- Treat key driver values 1-16 as matrix positions from left to right, top to bottom.
- Match the keypad labels from the provided image:
  - 1, 2, 3, password
  - 4, 5, 6, fingerprint
  - 7, 8, 9, card
  - clear, 0, backspace, confirm
- Fingerprint and card keys are ignored for now.
- Implement password input only inside `key_task()`.
- Display pressed digits in the LCD area `x=20..107`, `y=30..57`.
- Support backspace, clear, and confirm.
- On confirm, call `lock(0)` only when the password is correct.
- Initial password is `123456`.
