Our graphics course project implementation.
Based on a custom graphic engine based on Libigl.

# Difficulties
We had a hard time working with the engine, it was messy. Most of our time went into refactoring and fixing small bugs that took some time to find.
The features themselves were reasonable given the proper backend code to support it.

# Additional Features and refactors
* Window sections
  * Section layers
* Camera translations according to it's reference frame
* Pick Bezier segment
* etc.

# Compiling
1. Make sure CMake is installed
2. Run Cmake GUI:
   1. In the source code input, enter the path to where you cloned the repository.
   2. In build path input, enter a path to your choosing (/build from the repository's root is good).
   3. Click 'Configure' and wait for it to finish.
   4. Click it *AGAIN*
   5. Click 'Generate'.
3. The above has overridden some local files in the `external` directory. In order to get our version, Either:
  * Run `git restore *` in the directory of your local repository copy.
  * Copy the files from the `external` directory of the repository to the your local copy of the repository.
