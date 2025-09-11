# expert-system

An expert system in proposal calculation. In other words, a program that can
reason on a set of rules and initial facts to deduce other facts.

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

