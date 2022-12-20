def get_version() -> str:
    version = "0.0.0"
    try:
        from setuptools_scm import get_version

        version = get_version()
    except:
        pass

    return version


if __name__ == "__main__":
    print(get_version())
