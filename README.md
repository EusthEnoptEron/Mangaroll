# Mangaroll

Uses a cylindrical layout that lets the user progress his reading by turning around in a circle. Thanks to the GearVR being cablefree, there is no risk of getting yourself tangled up! Unless you're wearing a pair of headphones...

![](http://www.zomg.ch/mangaroll.jpg)

## Services

It's possible to integrate simple online services that will serve manga. To get this to work, the app needs two links: one to browse manga, and one to view a certain one.

### Browse view

**Required parameters in URL string**:
  - `{page}`
  - `{id}`


```json
{
  "success": true,
  "hasMore": true,
  "mangaList": [
    {
      "name": "Some manga magazine",
      "id"  : 1,
      "thumb": "http://thumbnail.jpg",
      "container": false
    },
    {
      "name": "Some manga that will have chapters",
      "id"  : 1,
      "thumb": "http://thumbnail.jpg",
      "container": true
    }
  ]
}
```


### Read view

**Required parameters in URL string**:
  - `{id}`

```json
{
  "success": true,
  "images": [
    "http://link.to/p1.jpg",
    "http://link.to/p2.jpg"
  ]
}
```
