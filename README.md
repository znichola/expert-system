# expert-system

An expert system in proposal calculation. In other words, a program that can
reason on a set of rules and initial facts to deduce other facts.

## Quick start

Quick start with cli tool.
```bash
make all
./expert-system --explain 00_socrates.txt
```

Server mod + graphviz for node graph visualisation
```bash
# download graphviz bundle
make external_deps/graphviz-13.1.2
# build graph viz in $HOME/graphviz
make graphviz
# rebuild
make re

./expert-system --explain --server 00_socrates.txt
# open a browser to localhost:7711
```

> see `dockerfile` for docker build & publish instructions

A version of the app is deployed with onrender on [expert-system-chc0.onrender.com/](https://expert-system-chc0.onrender.com/).

## Useful links


- [backwards chaining engine](https://en.wikipedia.org/wiki/Backward_chaining)
- [recursive decent parsing](https://www.youtube.com/watch?v=dDtZLm7HIJs)
- [regex explainer](https://regexr.com/)

## TODO for solver

- [ ] rename Var.value to Var.label, it's confusing with the optional value

## The Digraph

Digrap because it's directional. We have two types of nodes, `Fact` and `Rule`
a `Rule` can only be linked to other`Fact`/s.

### Fact

Internally this stores the list of rules used to deduce it. 

### Rule

Internally this stores the list of rules this deduces.

### Data Structure

We have two lists, Facts, and Rules, each is uniquly accessible.

Facts are indexed on the letter? A hashmap between letter & fact\_struct

Rules are index on ... a unique number given to them on creation?
or the printed string that reprisents their ast, this gives automatic
deduplication.

So, a hashmap between [ast\_string or id] & rule\_struct


## Graphviz

[Graphviz](https://graphviz.org/) is a prgram to display graphs, and it can also be included in my c++ project. We will use it to display the generated graph in a webserver.

[Using graphviz as a library](https://graphviz.org/docs/library/)

We want to install it as a static lib, we will then link this to the project and use it!

In a folder somewhere download the tar for the release, unpackage it, configure, then make, then insall (put the resulting lib & binaries somewhere).

These make commands are useful to do just this.

```bash
# download the release and put it in the external_deps folder
make external_deps/graphviz-13.1.2
# build the graphviz project and install it into GV_PREFIX = $(HOME)/graphviz
make graphviz
```
ps. Graphvis uses a very strange system for dynamically loading different parts of the pipeline, it's simplest to install it as dynamically linked lib, this is their default. I've lost many hours trying to link it staticaly, the problem is registering the static libs so at runtime it actually works and can the png, layout .. etc engine. 
