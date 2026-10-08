# Checks a family of walks of length 3 in B(4,n) independently of the C++ code.
# usage: python3 check/check_walk_family.py data/walk_cover_n6.txt [n]
import itertools
import sys

COMPLEMENT = {"A": "T", "T": "A", "C": "G", "G": "C"}


def rho(word):
    return "".join(COMPLEMENT[letter] for letter in reversed(word))


def orbit(word):
    return min(word, rho(word))


def check(path, n):
    walks = [line.strip() for line in open(path)
             if line.strip() and not line.startswith("#")]

    all_orbits = {orbit(e) for e in map("".join, itertools.product("ACGT", repeat=n))
                  if rho(e) != e}
    print(f"orbits in B(4,{n}): {len(all_orbits)} (formula: {(4**n - 4**(n // 2)) // 2})")
    print(f"walks in the file:  {len(walks)} (needed: {len(all_orbits) // 3})")

    used = []
    for number, walk in enumerate(walks, 1):
        if len(walk) != n + 2 or set(walk) - set("ACGT"):
            return f"walk {number} ({walk}) is not a word of length {n + 2} over ACGT"
        # the three edges of a walk of length 3 are its three windows of length n
        for edge in (walk[0:n], walk[1:n + 1], walk[2:n + 2]):
            if rho(edge) == edge:
                return f"walk {number} ({walk}) has a self-complementary edge {edge}"
            used.append(orbit(edge))

    if len(used) != len(set(used)):
        repeated = next(o for o in used if used.count(o) > 1)
        return f"orbit {repeated} is used more than once"
    if set(used) != all_orbits:
        return f"{len(all_orbits - set(used))} orbits are not used"
    return None


if __name__ == "__main__":
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    error = check(sys.argv[1], n)
    print("FAIL: " + error if error else "OK: every orbit is used exactly once")
    sys.exit(1 if error else 0)
