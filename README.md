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
    type     : String (e.g. 'window')
    , price  : Number (e.g. 50)
    , advice : String (constant, e.g. kBeCareful/kGoodJob/kHolyShit)
    , old    : Number ($/day)
    , new    : Number ($/day)
    , diff   : Number ($/day)
    , time?  : Number/String (e.g. trend7)
}
```
