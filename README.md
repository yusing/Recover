# Recover

### A tool to find and attempt to fix deleted Microsoft Office documents in a raw disk image.

---

### Requirements

- A linux machine (or build it yourself in Windows)
- A raw disk image
- Your brain

### How to use

1. Clone the repository

    ```bash
    git clone https://github.com/yusing/Recover
    ```

2. Change directory to `Recover`

    ```bash
    cd Recover
    ```

3. Run `recover` and wait for it to run.

    ```bash
    build/recover $PATH_TO_DISK_IMAGE
    ```

    Recovered documents are stored in `$PWD/found`

4. (optional) if you cannot open up the recovered document, run `fix_corrupted.sh`

    ```bash
    ./fix_corrupted.sh
    ```

    Recovered documents are stored in `$PWD/found` with prefix "fixed_", e.g. `fixed_mydoc.docx`

    Note: If you still cannot open up some documents after this, that means those document are totally corrupted. R.I.P. 🥺