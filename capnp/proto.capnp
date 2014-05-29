@0x814011bf5f2fb1dd;

struct ArgList {
    type @0 :Type;
    args @1 :List(Arg);

    enum Type {
      left @0;
      right @1;
    }

    struct Arg{
     data @0 :Data;
     size @1 :UInt32;
    }
}

interface Call {
  call @0 (in: ArgList, out_size: Int64) -> (out: ArgList);
}
