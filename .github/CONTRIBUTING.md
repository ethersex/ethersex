# How to Contribute

We are a truly open source community and therefore appreciate any effort to make ethersex even more awesome. You can help in various ways like writing code, supporting other users (e.g. on IRC) or expanding this wiki with your knowledge.


## Writing Code

All your code needs to be [*GPLv3* or later](http://www.gnu.org/copyleft/gpl.html) if you plan to publish your code.


## Coding Style

To keep the source code readable some formatting rules should be adhered to. There is further evidence found in [adding your own module](http://www.ethersex.de/index.php/Own_module).

The supplied script `scripts/indent.sh` may be used to format the source code before committing it to the repository. Please keep in mind that pull requests might be rejected if source violates formatting rules.

  *  GPLv3 license header if none exists
  *  Maximum line length is 78 chars.
  *  No text editor specific formatting rules.
  *  Use spaces instead of tabs, two spaces per indent level.
  *  Braces on single line, no additional indent.
  *  No spaces between the name of the procedure being called and the `('.
  *  Put the `*' character at the left of comments.
  *  Break long lines after operators.
  *  Macros are defined just below the includes 

Function prototypes are defined as.

        void func(void);

and implemented as

        void
        func(void)
        {
          /* content */
        }


## Fork us!

For version control we use [Git](http://git-scm.com/). Our repository is hosted on [github](https://github.com/ethersex/ethersex).

If you haven't work with git so far, you should familiarize yourself with it by reading the [beginners guide on github](https://help.github.com/).

The [help page on github](https://help.github.com/articles/fork-a-repo/) pretty much explains how that works.

To make your life easier you should create a branch for each new feature you will be implementing. NEVER touch the master branch - or you probably will not be able to merge upstream changes into your fork without conflicts.


## Pull requests

Pull requests are the easiest way to get your code integrated into the ethersex code base. Developers don't need to send patches around on mailing lists and can respond faster to questions/concerns maintainers may have about the code.

Please read this page in case you don't know how to use pull requests on [github](https://help.github.com/articles/about-pull-requests/).

