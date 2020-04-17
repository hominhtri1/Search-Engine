# Search-Engine
A C++ console application that allow users to search for a word, a phrase, or more complex queries from a set of text files.

Features include:
  * Users can use operators such as NOT, AND, OR to refine their search.
  * The application also searches for synonyms of provided words.
  * Users can put ‘*’ in the query to indicate wildcards or unknown words.
  * The application stores previous searches and suggests these to the users while they are typing.
  * The returned files are ranked according to the frequencies and densities of keywords in the files.

The above features are provided using various data structures such as B-trees, radix trees, and suffix trees.
