# MMU LAB
## compile
already compiled
the excutable file is located on './build/mmu'

if you want to recompile

run the command
```bash
cd build
make
```

## run

```bash
cd lab3_assign
./runit.sh ../outputs/ ../build/mmu
```

## grade

```bash
cd lab3_assign
./gradeit.sh ./refout/ ../outputs/
```

## test result

test result on my computer is below
```bash
$ ./gradeit.sh ./refout/ ../outputs/
input  frames    f  r  c  e  a  w
1       16       .  .  .  .  .  .
1       32       .  .  .  .  .  .
2       16       .  .  .  .  .  .
2       32       .  .  .  .  .  .
3       16       .  .  .  .  .  .
3       32       .  .  .  .  .  .
4       16       .  .  .  .  .  .
4       32       .  .  .  .  .  .
5       16       .  .  .  .  .  .
5       32       .  .  .  .  .  .
6       16       .  .  .  .  .  .
6       32       .  .  .  .  .  .
7       16       .  .  .  .  .  .
7       32       .  .  .  .  .  .
8       16       .  .  .  .  .  .
8       32       .  .  .  .  .  .
9       16       .  .  .  .  .  .
9       32       .  .  .  .  .  .
10      16       .  .  .  .  .  .
10      32       .  .  .  .  .  .
11      16       .  .  .  .  .  .
11      32       .  .  .  .  .  .
SUM             22 22 22 22 22 22
```


