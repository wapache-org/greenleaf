--  distributions have the following format:
--
--  <token> | <weight> # comment
--
--  Distributions are used to bias the selection of a token
--  based on its associated weight. The list of tokens and values
--  between the keywords BEGIN and END define the distribution named after
--  the BEGIN. A uniformly random value from [0, sum(weights)]
--  will be chosen and the first token whose cumulative weight is greater than
--  or equal to the result will be returned. In essence, the weights for each
--  token represent its relative weight within a distribution.
--
--  one special token is defined: count (number of data points in the
--   distribution). It MUST be defined for each named distribution.
-- -----------------------------------------------------------------------
--  currently defined distributions and their use:
--   NAME       FIELD/NOTES
--   ========   ==============
--   category   parts.category
--   container  parts.container
--   instruct   shipping instructions
--   msegmnt    market segment
--   names      parts.name
--   nations    must be ordered along with regions
--   nations2   stand alone nations set for use with qgen
--   o_prio     order priority
--   regions    must be ordered along with nations
--   rflag      lineitems.returnflag
--   types      parts.type
--   colors     embedded string creation; CANNOT BE USED FOR pick_str(), agg_str() perturbs order
--   articles   comment generation
--   nouns
--   verbs
--   adverbs
--   auxillaries
--   prepositions
--   terminators
--   grammar    sentence formation
--   np
--   vp
--

insert into distribution
(d_type, d_token, d_weight)
VALUES

--###
--# category
--###
--BEGIN category
--COUNT|5
('category', 'FURNITURE', 1),
('category', 'STORAGE EQUIP', 1),
('category', 'TOOLS', 1),
('category', 'MACHINE TOOLS', 1),
('category', 'OTHER', 1),

--###
--# container
--###
--begin p_cntr
--count|40
('p_cntr', 'SM CASE',1   ),
('p_cntr', 'SM BOX',1    ),
('p_cntr', 'SM BAG',1    ),
('p_cntr', 'SM JAR',1    ),
('p_cntr', 'SM PACK',1   ),
('p_cntr', 'SM PKG',1    ),
('p_cntr', 'SM CAN',1    ),
('p_cntr', 'SM DRUM',1   ),
('p_cntr', 'LG CASE',1   ),
('p_cntr', 'LG BOX',1    ),
('p_cntr', 'LG BAG',1    ),
('p_cntr', 'LG JAR',1    ),
('p_cntr', 'LG PACK',1   ),
('p_cntr', 'LG PKG',1    ),
('p_cntr', 'LG CAN',1    ),
('p_cntr', 'LG DRUM',1   ),
('p_cntr', 'MED CASE',1  ),
('p_cntr', 'MED BOX',1   ),
('p_cntr', 'MED BAG',1   ),
('p_cntr', 'MED JAR',1   ),
('p_cntr', 'MED PACK',1  ),
('p_cntr', 'MED PKG',1   ),
('p_cntr', 'MED CAN',1   ),
('p_cntr', 'MED DRUM',1  ),
('p_cntr', 'JUMBO CASE',1),
('p_cntr', 'JUMBO BOX',1 ),
('p_cntr', 'JUMBO BAG',1 ),
('p_cntr', 'JUMBO JAR',1 ),
('p_cntr', 'JUMBO PACK',1),
('p_cntr', 'JUMBO PKG',1 ),
('p_cntr', 'JUMBO CAN',1 ),
('p_cntr', 'JUMBO DRUM',1),
('p_cntr', 'WRAP CASE',1 ),
('p_cntr', 'WRAP BOX',1  ),
('p_cntr', 'WRAP BAG',1  ),
('p_cntr', 'WRAP JAR',1  ),
('p_cntr', 'WRAP PACK',1 ),
('p_cntr', 'WRAP PKG',1  ),
('p_cntr', 'WRAP CAN',1  ),
('p_cntr', 'WRAP DRUM',1 ),

--###
--# instruct
--###
--begin instruct
--count|4
('instruct', 'DELIVER IN PERSON', 1),
('instruct', 'COLLECT COD', 1),
('instruct', 'TAKE BACK RETURN', 1),
('instruct', 'NONE', 1),

--###
--# msegmnt
--###
--begin msegmnt
--count|5
('msegmnt', 'AUTOMOBILE', 1),
('msegmnt', 'BUILDING', 1),
('msegmnt', 'FURNITURE', 1),
('msegmnt', 'HOUSEHOLD', 1),
('msegmnt', 'MACHINERY', 1),

--###
--# names
--###
--begin p_names
--COUNT|4
('p_names', 'CLEANER', 1),
('p_names', 'SOAP', 1),
('p_names', 'DETERGENT', 1),
('p_names', 'EXTRA', 1),

--###
--# nations
--# NOTE: this is a special case; the weights here are adjustments to
--#       map correctly into the regions table, and are *NOT* cummulative
--#       values to mimic a distribution
--###
--begin nations
--count|25
('nations', 'ALGERIA',0       ),
('nations', 'ARGENTINA',1     ),
('nations', 'BRAZIL',0        ),
('nations', 'CANADA',0        ),
('nations', 'EGYPT',3         ),
('nations', 'ETHIOPIA',-4     ),
('nations', 'FRANCE',3        ),
('nations', 'GERMANY',0       ),
('nations', 'INDIA',-1        ),
('nations', 'INDONESIA',0     ),
('nations', 'IRAN',2          ),
('nations', 'IRAQ',0          ),
('nations', 'JAPAN',-2        ),
('nations', 'JORDAN',2        ),
('nations', 'KENYA',-4        ),
('nations', 'MOROCCO',0       ),
('nations', 'MOZAMBIQUE',0    ),
('nations', 'PERU',1          ),
('nations', 'CHINA',1         ),
('nations', 'ROMANIA',1       ),
('nations', 'SAUDI ARABIA',1  ),
('nations', 'VIETNAM',-2      ),
('nations', 'RUSSIA',1        ),
('nations', 'UNITED KINGDOM',0),
('nations', 'UNITED STATES',-2),

--###
--# nations2
--###
--begin nations2
--count|25
('nations2', 'ALGERIA',1        ),
('nations2', 'ARGENTINA',1      ),
('nations2', 'BRAZIL',1         ),
('nations2', 'CANADA',1         ),
('nations2', 'EGYPT',1          ),
('nations2', 'ETHIOPIA',1       ),
('nations2', 'FRANCE',1         ),
('nations2', 'GERMANY',1        ),
('nations2', 'INDIA',1          ),
('nations2', 'INDONESIA',1      ),
('nations2', 'IRAN',1           ),
('nations2', 'IRAQ',1           ),
('nations2', 'JAPAN',1          ),
('nations2', 'JORDAN',1         ),
('nations2', 'KENYA',1          ),
('nations2', 'MOROCCO',1        ),
('nations2', 'MOZAMBIQUE',1     ),
('nations2', 'PERU',1           ),
('nations2', 'CHINA',1          ),
('nations2', 'ROMANIA',1        ),
('nations2', 'SAUDI ARABIA',1   ),
('nations2', 'VIETNAM',1        ),
('nations2', 'RUSSIA',1         ),
('nations2', 'UNITED KINGDOM',1 ),
('nations2', 'UNITED STATES',1  ),

--###
--# regions
--###
--begin regions
--count|5
('regions', 'AFRICA', 1),
('regions', 'AMERICA', 1),
('regions', 'ASIA', 1),
('regions', 'EUROPE', 1),
('regions', 'MIDDLE EAST', 1),
--###
--# o_prio
--###
--begin o_oprio
--count|5
('o_prio', '1-URGENT', 1),
('o_prio', '2-HIGH', 1),
('o_prio', '3-MEDIUM', 1),
('o_prio', '4-NOT SPECIFIED', 1),
('o_prio', '5-LOW', 1),
--###
--# rflag
--###
--begin rflag
--count|2
('rflag', 'R', 1),
('rflag', 'A', 1),
--###
--# smode
--###
--begin smode
--count|7
('smode', 'REG AIR', 1),
('smode', 'AIR', 1),
('smode', 'RAIL', 1),
('smode', 'TRUCK', 1),
('smode', 'MAIL', 1),
('smode', 'FOB', 1),
('smode', 'SHIP', 1),

--###
--# types
--###
--begin p_types
--COUNT|150
('p_types', 'STANDARD ANODIZED TIN',1     ),
('p_types', 'STANDARD ANODIZED NICKEL',1  ),
('p_types', 'STANDARD ANODIZED BRASS',1   ),
('p_types', 'STANDARD ANODIZED STEEL',1   ),
('p_types', 'STANDARD ANODIZED COPPER',1  ),
('p_types', 'STANDARD BURNISHED TIN',1    ),
('p_types', 'STANDARD BURNISHED NICKEL',1 ),
('p_types', 'STANDARD BURNISHED BRASS',1  ),
('p_types', 'STANDARD BURNISHED STEEL',1  ),
('p_types', 'STANDARD BURNISHED COPPER',1 ),
('p_types', 'STANDARD PLATED TIN',1       ),
('p_types', 'STANDARD PLATED NICKEL',1    ),
('p_types', 'STANDARD PLATED BRASS',1     ),
('p_types', 'STANDARD PLATED STEEL',1     ),
('p_types', 'STANDARD PLATED COPPER',1    ),
('p_types', 'STANDARD POLISHED TIN',1     ),
('p_types', 'STANDARD POLISHED NICKEL',1  ),
('p_types', 'STANDARD POLISHED BRASS',1   ),
('p_types', 'STANDARD POLISHED STEEL',1   ),
('p_types', 'STANDARD POLISHED COPPER',1  ),
('p_types', 'STANDARD BRUSHED TIN',1      ),
('p_types', 'STANDARD BRUSHED NICKEL',1   ),
('p_types', 'STANDARD BRUSHED BRASS',1    ),
('p_types', 'STANDARD BRUSHED STEEL',1    ),
('p_types', 'STANDARD BRUSHED COPPER',1   ),
('p_types', 'SMALL ANODIZED TIN',1        ),
('p_types', 'SMALL ANODIZED NICKEL',1     ),
('p_types', 'SMALL ANODIZED BRASS',1      ),
('p_types', 'SMALL ANODIZED STEEL',1      ),
('p_types', 'SMALL ANODIZED COPPER',1     ),
('p_types', 'SMALL BURNISHED TIN',1       ),
('p_types', 'SMALL BURNISHED NICKEL',1    ),
('p_types', 'SMALL BURNISHED BRASS',1     ),
('p_types', 'SMALL BURNISHED STEEL',1     ),
('p_types', 'SMALL BURNISHED COPPER',1    ),
('p_types', 'SMALL PLATED TIN',1          ),
('p_types', 'SMALL PLATED NICKEL',1       ),
('p_types', 'SMALL PLATED BRASS',1        ),
('p_types', 'SMALL PLATED STEEL',1        ),
('p_types', 'SMALL PLATED COPPER',1       ),
('p_types', 'SMALL POLISHED TIN',1        ),
('p_types', 'SMALL POLISHED NICKEL',1     ),
('p_types', 'SMALL POLISHED BRASS',1      ),
('p_types', 'SMALL POLISHED STEEL',1      ),
('p_types', 'SMALL POLISHED COPPER',1     ),
('p_types', 'SMALL BRUSHED TIN',1         ),
('p_types', 'SMALL BRUSHED NICKEL',1      ),
('p_types', 'SMALL BRUSHED BRASS',1       ),
('p_types', 'SMALL BRUSHED STEEL',1       ),
('p_types', 'SMALL BRUSHED COPPER',1      ),
('p_types', 'MEDIUM ANODIZED TIN',1       ),
('p_types', 'MEDIUM ANODIZED NICKEL',1    ),
('p_types', 'MEDIUM ANODIZED BRASS',1     ),
('p_types', 'MEDIUM ANODIZED STEEL',1     ),
('p_types', 'MEDIUM ANODIZED COPPER',1    ),
('p_types', 'MEDIUM BURNISHED TIN',1      ),
('p_types', 'MEDIUM BURNISHED NICKEL',1   ),
('p_types', 'MEDIUM BURNISHED BRASS',1    ),
('p_types', 'MEDIUM BURNISHED STEEL',1    ),
('p_types', 'MEDIUM BURNISHED COPPER',1   ),
('p_types', 'MEDIUM PLATED TIN',1         ),
('p_types', 'MEDIUM PLATED NICKEL',1      ),
('p_types', 'MEDIUM PLATED BRASS',1       ),
('p_types', 'MEDIUM PLATED STEEL',1       ),
('p_types', 'MEDIUM PLATED COPPER',1      ),
('p_types', 'MEDIUM POLISHED TIN',1       ),
('p_types', 'MEDIUM POLISHED NICKEL',1    ),
('p_types', 'MEDIUM POLISHED BRASS',1     ),
('p_types', 'MEDIUM POLISHED STEEL',1     ),
('p_types', 'MEDIUM POLISHED COPPER',1    ),
('p_types', 'MEDIUM BRUSHED TIN',1        ),
('p_types', 'MEDIUM BRUSHED NICKEL',1     ),
('p_types', 'MEDIUM BRUSHED BRASS',1      ),
('p_types', 'MEDIUM BRUSHED STEEL',1      ),
('p_types', 'MEDIUM BRUSHED COPPER',1     ),
('p_types', 'LARGE ANODIZED TIN',1        ),
('p_types', 'LARGE ANODIZED NICKEL',1     ),
('p_types', 'LARGE ANODIZED BRASS',1      ),
('p_types', 'LARGE ANODIZED STEEL',1      ),
('p_types', 'LARGE ANODIZED COPPER',1     ),
('p_types', 'LARGE BURNISHED TIN',1       ),
('p_types', 'LARGE BURNISHED NICKEL',1    ),
('p_types', 'LARGE BURNISHED BRASS',1     ),
('p_types', 'LARGE BURNISHED STEEL',1     ),
('p_types', 'LARGE BURNISHED COPPER',1    ),
('p_types', 'LARGE PLATED TIN',1          ),
('p_types', 'LARGE PLATED NICKEL',1       ),
('p_types', 'LARGE PLATED BRASS',1        ),
('p_types', 'LARGE PLATED STEEL',1        ),
('p_types', 'LARGE PLATED COPPER',1       ),
('p_types', 'LARGE POLISHED TIN',1        ),
('p_types', 'LARGE POLISHED NICKEL',1     ),
('p_types', 'LARGE POLISHED BRASS',1      ),
('p_types', 'LARGE POLISHED STEEL',1      ),
('p_types', 'LARGE POLISHED COPPER',1     ),
('p_types', 'LARGE BRUSHED TIN',1         ),
('p_types', 'LARGE BRUSHED NICKEL',1      ),
('p_types', 'LARGE BRUSHED BRASS',1       ),
('p_types', 'LARGE BRUSHED STEEL',1       ),
('p_types', 'LARGE BRUSHED COPPER',1      ),
('p_types', 'ECONOMY ANODIZED TIN',1      ),
('p_types', 'ECONOMY ANODIZED NICKEL',1   ),
('p_types', 'ECONOMY ANODIZED BRASS',1    ),
('p_types', 'ECONOMY ANODIZED STEEL',1    ),
('p_types', 'ECONOMY ANODIZED COPPER',1   ),
('p_types', 'ECONOMY BURNISHED TIN',1     ),
('p_types', 'ECONOMY BURNISHED NICKEL',1  ),
('p_types', 'ECONOMY BURNISHED BRASS',1   ),
('p_types', 'ECONOMY BURNISHED STEEL',1   ),
('p_types', 'ECONOMY BURNISHED COPPER',1  ),
('p_types', 'ECONOMY PLATED TIN',1        ),
('p_types', 'ECONOMY PLATED NICKEL',1     ),
('p_types', 'ECONOMY PLATED BRASS',1      ),
('p_types', 'ECONOMY PLATED STEEL',1      ),
('p_types', 'ECONOMY PLATED COPPER',1     ),
('p_types', 'ECONOMY POLISHED TIN',1      ),
('p_types', 'ECONOMY POLISHED NICKEL',1   ),
('p_types', 'ECONOMY POLISHED BRASS',1    ),
('p_types', 'ECONOMY POLISHED STEEL',1    ),
('p_types', 'ECONOMY POLISHED COPPER',1   ),
('p_types', 'ECONOMY BRUSHED TIN',1       ),
('p_types', 'ECONOMY BRUSHED NICKEL',1    ),
('p_types', 'ECONOMY BRUSHED BRASS',1     ),
('p_types', 'ECONOMY BRUSHED STEEL',1     ),
('p_types', 'ECONOMY BRUSHED COPPER',1    ),
('p_types', 'PROMO ANODIZED TIN',1        ),
('p_types', 'PROMO ANODIZED NICKEL',1     ),
('p_types', 'PROMO ANODIZED BRASS',1      ),
('p_types', 'PROMO ANODIZED STEEL',1      ),
('p_types', 'PROMO ANODIZED COPPER',1     ),
('p_types', 'PROMO BURNISHED TIN',1       ),
('p_types', 'PROMO BURNISHED NICKEL',1    ),
('p_types', 'PROMO BURNISHED BRASS',1     ),
('p_types', 'PROMO BURNISHED STEEL',1     ),
('p_types', 'PROMO BURNISHED COPPER',1    ),
('p_types', 'PROMO PLATED TIN',1          ),
('p_types', 'PROMO PLATED NICKEL',1       ),
('p_types', 'PROMO PLATED BRASS',1        ),
('p_types', 'PROMO PLATED STEEL',1        ),
('p_types', 'PROMO PLATED COPPER',1       ),
('p_types', 'PROMO POLISHED TIN',1        ),
('p_types', 'PROMO POLISHED NICKEL',1     ),
('p_types', 'PROMO POLISHED BRASS',1      ),
('p_types', 'PROMO POLISHED STEEL',1      ),
('p_types', 'PROMO POLISHED COPPER',1     ),
('p_types', 'PROMO BRUSHED TIN',1         ),
('p_types', 'PROMO BRUSHED NICKEL',1      ),
('p_types', 'PROMO BRUSHED BRASS',1       ),
('p_types', 'PROMO BRUSHED STEEL',1       ),
('p_types', 'PROMO BRUSHED COPPER',1      ),

--###
--# colors
--# NOTE: This distribution CANNOT be used by pick_str(), since agg_str() perturbs its order
--###
--begin colors
--COUNT|92
('colors', 'almond',1     ),
('colors', 'antique',1    ),
('colors', 'aquamarine',1 ),
('colors', 'azure',1      ),
('colors', 'beige',1      ),
('colors', 'bisque',1     ),
('colors', 'black',1      ),
('colors', 'blanched',1   ),
('colors', 'blue',1       ),
('colors', 'blush',1      ),
('colors', 'brown',1      ),
('colors', 'burlywood',1  ),
('colors', 'burnished',1  ),
('colors', 'chartreuse',1 ),
('colors', 'chiffon',1    ),
('colors', 'chocolate',1  ),
('colors', 'coral',1      ),
('colors', 'cornflower',1 ),
('colors', 'cornsilk',1   ),
('colors', 'cream',1      ),
('colors', 'cyan',1       ),
('colors', 'dark',1       ),
('colors', 'deep',1       ),
('colors', 'dim',1        ),
('colors', 'dodger',1     ),
('colors', 'drab',1       ),
('colors', 'firebrick',1  ),
('colors', 'floral',1     ),
('colors', 'forest',1     ),
('colors', 'frosted',1    ),
('colors', 'gainsboro',1  ),
('colors', 'ghost',1      ),
('colors', 'goldenrod',1  ),
('colors', 'green',1      ),
('colors', 'grey',1       ),
('colors', 'honeydew',1   ),
('colors', 'hot',1        ),
('colors', 'indian',1     ),
('colors', 'ivory',1      ),
('colors', 'khaki',1      ),
('colors', 'lace',1       ),
('colors', 'lavender',1   ),
('colors', 'lawn',1       ),
('colors', 'lemon',1      ),
('colors', 'light',1      ),
('colors', 'lime',1       ),
('colors', 'linen',1      ),
('colors', 'magenta',1    ),
('colors', 'maroon',1     ),
('colors', 'medium',1     ),
('colors', 'metallic',1   ),
('colors', 'midnight',1   ),
('colors', 'mint',1       ),
('colors', 'misty',1      ),
('colors', 'moccasin',1   ),
('colors', 'navajo',1     ),
('colors', 'navy',1       ),
('colors', 'olive',1      ),
('colors', 'orange',1     ),
('colors', 'orchid',1     ),
('colors', 'pale',1       ),
('colors', 'papaya',1     ),
('colors', 'peach',1      ),
('colors', 'peru',1       ),
('colors', 'pink',1       ),
('colors', 'plum',1       ),
('colors', 'powder',1     ),
('colors', 'puff',1       ),
('colors', 'purple',1     ),
('colors', 'red',1        ),
('colors', 'rose',1       ),
('colors', 'rosy',1       ),
('colors', 'royal',1      ),
('colors', 'saddle',1     ),
('colors', 'salmon',1     ),
('colors', 'sandy',1      ),
('colors', 'seashell',1   ),
('colors', 'sienna',1     ),
('colors', 'sky',1        ),
('colors', 'slate',1      ),
('colors', 'smoke',1      ),
('colors', 'snow',1       ),
('colors', 'spring',1     ),
('colors', 'steel',1      ),
('colors', 'tan',1        ),
('colors', 'thistle',1    ),
('colors', 'tomato',1     ),
('colors', 'turquoise',1  ),
('colors', 'violet',1     ),
('colors', 'wheat',1      ),
('colors', 'white',1      ),
('colors', 'yellow',1     ),

--################
--################
--## psuedo text distributions
--################
--################
--###
--# nouns
--###
--BEGIN nouns
--COUNT|45
('nouns', 'packages',40      ),
('nouns', 'requests',40      ),
('nouns', 'accounts',40      ),
('nouns', 'deposits',40      ),
('nouns', 'foxes',20         ),
('nouns', 'ideas',20         ),
('nouns', 'theodolites',20   ),
('nouns', 'pinto beans',20   ),
('nouns', 'instructions',20  ),
('nouns', 'dependencies',10  ),
('nouns', 'excuses',10       ),
('nouns', 'platelets',10     ),
('nouns', 'asymptotes',10    ),
('nouns', 'courts',5         ),
('nouns', 'dolphins',5       ),
('nouns', 'multipliers',1    ),
('nouns', 'sauternes',1      ),
('nouns', 'warthogs',1       ),
('nouns', 'frets',1          ),
('nouns', 'dinos',1          ),
('nouns', 'attainments',1    ),
('nouns', 'somas',1          ),
('nouns', 'Tiresias',1       ),
('nouns', 'patterns',1       ),
('nouns', 'forges',1         ),
('nouns', 'braids',1         ),
('nouns', 'frays',1          ),
('nouns', 'warhorses',1      ),
('nouns', 'dugouts',1        ),
('nouns', 'notornis',1       ),
('nouns', 'epitaphs',1       ),
('nouns', 'pearls',1         ),
('nouns', 'tithes',1         ),
('nouns', 'waters',1         ),
('nouns', 'orbits',1         ),
('nouns', 'gifts',1          ),
('nouns', 'sheaves',1        ),
('nouns', 'depths',1         ),
('nouns', 'sentiments',1     ),
('nouns', 'decoys',1         ),
('nouns', 'realms',1         ),
('nouns', 'pains',1          ),
('nouns', 'grouches',1       ),
('nouns', 'escapades',1      ),
('nouns', 'hockey players',1 ),
--###
--# verbs
--###
--BEGIN verbs
--COUNT|40
('verbs', 'sleep',20    ),
('verbs', 'wake',20     ),
('verbs', 'are',20      ),
('verbs', 'cajole',20   ),
('verbs', 'haggle',20   ),
('verbs', 'nag',10      ),
('verbs', 'use',10      ),
('verbs', 'boost',10    ),
('verbs', 'affix',5     ),
('verbs', 'detect',5    ),
('verbs', 'integrate',5 ),
('verbs', 'maintain',1  ),
('verbs', 'nod',1       ),
('verbs', 'was',1       ),
('verbs', 'lose',1      ),
('verbs', 'sublate',1   ),
('verbs', 'solve',1     ),
('verbs', 'thrash',1    ),
('verbs', 'promise',1   ),
('verbs', 'engage',1    ),
('verbs', 'hinder',1    ),
('verbs', 'print',1     ),
('verbs', 'x-ray',1     ),
('verbs', 'breach',1    ),
('verbs', 'eat',1       ),
('verbs', 'grow',1      ),
('verbs', 'impress',1   ),
('verbs', 'mold',1      ),
('verbs', 'poach',1     ),
('verbs', 'serve',1     ),
('verbs', 'run',1       ),
('verbs', 'dazzle',1    ),
('verbs', 'snooze',1    ),
('verbs', 'doze',1      ),
('verbs', 'unwind',1    ),
('verbs', 'kindle',1    ),
('verbs', 'play',1      ),
('verbs', 'hang',1      ),
('verbs', 'believe',1   ),
('verbs', 'doubt',1     ),

--###
--# adverbs
--##
--BEGIN adverbs
--COUNT|28
('adverbs', 'sometimes',1   ),
('adverbs', 'always',1      ),
('adverbs', 'never',1       ),
('adverbs', 'furiously',50  ),
('adverbs', 'slyly',50      ),
('adverbs', 'carefully',50  ),
('adverbs', 'blithely',40   ),
('adverbs', 'quickly',30    ),
('adverbs', 'fluffily',20   ),
('adverbs', 'slowly',1      ),
('adverbs', 'quietly',1     ),
('adverbs', 'ruthlessly',1  ),
('adverbs', 'thinly',1      ),
('adverbs', 'closely',1     ),
('adverbs', 'doggedly',1    ),
('adverbs', 'daringly',1    ),
('adverbs', 'bravely',1     ),
('adverbs', 'stealthily',1  ),
('adverbs', 'permanently',1 ),
('adverbs', 'enticingly',1  ),
('adverbs', 'idly',1        ),
('adverbs', 'busily',1      ),
('adverbs', 'regularly',1   ),
('adverbs', 'finally',1     ),
('adverbs', 'ironically',1  ),
('adverbs', 'evenly',1      ),
('adverbs', 'boldly',1      ),
('adverbs', 'silently',1    ),

--###
--# articles
--##
--BEGIN articles
--COUNT|3
('articles', 'the', 50),
('articles', 'a', 20),
('articles', 'an', 5),

--###
--# prepositions
--##
--BEGIN prepositions
--COUNT|47
('prepositions', 'about',50        ),
('prepositions', 'above',50        ),
('prepositions', 'according to',50 ),
('prepositions', 'across',50       ),
('prepositions', 'after',50        ),
('prepositions', 'against',40      ),
('prepositions', 'along',40        ),
('prepositions', 'alongside of',30 ),
('prepositions', 'among',30        ),
('prepositions', 'around',20       ),
('prepositions', 'at',10           ),
('prepositions', 'atop',1          ),
('prepositions', 'before',1        ),
('prepositions', 'behind',1        ),
('prepositions', 'beneath',1       ),
('prepositions', 'beside',1        ),
('prepositions', 'besides',1       ),
('prepositions', 'between',1       ),
('prepositions', 'beyond',1        ),
('prepositions', 'by',1            ),
('prepositions', 'despite',1       ),
('prepositions', 'during',1        ),
('prepositions', 'except',1        ),
('prepositions', 'for',1           ),
('prepositions', 'from',1          ),
('prepositions', 'in place of',1   ),
('prepositions', 'inside',1        ),
('prepositions', 'instead of',1    ),
('prepositions', 'into',1          ),
('prepositions', 'near',1          ),
('prepositions', 'of',1            ),
('prepositions', 'on',1            ),
('prepositions', 'outside',1       ),
('prepositions', 'over',1          ),
('prepositions', 'past',1          ),
('prepositions', 'since',1         ),
('prepositions', 'through',1       ),
('prepositions', 'throughout',1    ),
('prepositions', 'to',1            ),
('prepositions', 'toward',1        ),
('prepositions', 'under',1         ),
('prepositions', 'until',1         ),
('prepositions', 'up',1            ),
('prepositions', 'upon',1          ),
('prepositions', 'whithout',1      ),
('prepositions', 'with',1          ),
('prepositions', 'within',1        ),

--###
--# auxillaries
--##
--BEGIN auxillaries
--COUNT|18
('auxillaries', 'do',1             ),
('auxillaries', 'may',1            ),
('auxillaries', 'might',1          ),
('auxillaries', 'shall',1          ),
('auxillaries', 'will',1           ),
('auxillaries', 'would',1          ),
('auxillaries', 'can',1            ),
('auxillaries', 'could',1          ),
('auxillaries', 'should',1         ),
('auxillaries', 'ought to',1       ),
('auxillaries', 'must',1           ),
('auxillaries', 'will have to',1   ),
('auxillaries', 'shall have to',1  ),
('auxillaries', 'could have to',1  ),
('auxillaries', 'should have to',1 ),
('auxillaries', 'must have to',1   ),
('auxillaries', 'need to',1        ),
('auxillaries', 'try to',1         ),

--###
--# terminators
--##
--BEGIN terminators
--COUNT|6
('terminators', '.', 50),
('terminators', ';', 1),
('terminators', ':', 1),
('terminators', '?', 1),
('terminators', '!', 1),
('terminators', '--', 1),

--###
--# adjectives
--##
--BEGIN adjectives
--COUNT|29
('adjectives', 'special',20  ),
('adjectives', 'pending',20  ),
('adjectives', 'unusual',20  ),
('adjectives', 'express',20  ),
('adjectives', 'furious',1   ),
('adjectives', 'sly',1       ),
('adjectives', 'careful',1   ),
('adjectives', 'blithe',1    ),
('adjectives', 'quick',1     ),
('adjectives', 'fluffy',1    ),
('adjectives', 'slow',1      ),
('adjectives', 'quiet',1     ),
('adjectives', 'ruthless',1  ),
('adjectives', 'thin',1      ),
('adjectives', 'close',1     ),
('adjectives', 'dogged',1    ),
('adjectives', 'daring',1    ),
('adjectives', 'brave',1     ),
('adjectives', 'stealthy',1  ),
('adjectives', 'permanent',1 ),
('adjectives', 'enticing',1  ),
('adjectives', 'idle',1      ),
('adjectives', 'busy',1      ),
('adjectives', 'regular',50  ),
('adjectives', 'final',40    ),
('adjectives', 'ironic',40   ),
('adjectives', 'even',30     ),
('adjectives', 'bold',20     ),
('adjectives', 'silent',10   ),

--###
--# grammar
--# first level grammar. N=noun phrase, V=verb phrase,
--# P=prepositional phrase, T=setence termination
--##
--BEGIN grammar
--COUNT|5
('grammar', 'N V T', 3),
('grammar', 'N V P T', 3),
('grammar', 'N V N T', 3),
('grammar', 'N P V N T', 1),
('grammar', 'N P V P T', 1),

--###
--# NP
--# second level grammar. Noun phrases. N=noun, A=article,
--# J=adjective, D=adverb
--##
--BEGIN np
--COUNT|4
('np', 'N', 10),
('np', 'J N', 20),
('np', 'J, J N', 10),
('np', 'D J N', 50),

--###
--# VP
--# second level grammar. Verb phrases. V=verb, X=auxiallary,
--# D=adverb
--##
--BEGIN vp
--COUNT|4
('vp', 'V', 30),
('vp', 'X V', 1),
('vp', 'V D', 40),
('vp', 'X V D', 1),

--###
--# Q13
--# Substitution parameters for Q13
--##
--BEGIN Q13a
--COUNT|4
('Q13a', 'special', 20),
('Q13a', 'pending', 20),
('Q13a', 'unusual', 20),
('Q13a', 'express', 20),

--BEGIN Q13b
--COUNT|4
('Q13b', 'packages', 40),
('Q13b', 'requests', 40),
('Q13b', 'accounts', 40),
('Q13b', 'deposits', 40)
;
