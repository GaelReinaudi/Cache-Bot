# CacheBot README.md
## Conversation flows
### Prompts
Prompts are notifications sent by Cache, that may or may not include a question (preferably yes/no)
#### Income prompts

### Yes/no buying decisions
### Advice
#### Types
##### Price windows
```
{
    type         : String (e.g. 'window')
    , price      : Number (e.g. 50)
    , fact       : String (constant, e.g. chess notation ?+, ? is bad, ! is good, +/- is in/outcome)
    , factStr    : String (temporary human readable version of the "fact")
    , oldDaily   : Number ($/day)
    , newDaily   : Number ($/day)
    , difDaily   : Number ($/day)
    , overDays?  : Number (e.g. 7 for a trend7)
}
```
