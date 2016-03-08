# Mangaroll

Uses a cylindrical layout that lets the user progress his reading by turning around in a circle. Thanks to the GearVR being cablefree, there is no risk of getting yourself tangled up! Unless you're wearing a pair of headphones...

![](http://www.zomg.ch/mangaroll.jpg)

## Services

It's possible to integrate simple online services that will serve manga. To get this to work, the app needs two links: one to browse manga, and one to view a certain one.

### Browse view

**Parameters for URL string**:
  - `%d` = page


```json
{
  "success": true,
  "hasMore": true,
  "mangaList": [
    {
      "name": "Name",
      "id"  : 1,
      "thumb": "http://thumbnail.jpg"
    }
  ]
}
```


### Read view

**Parameters for URL string**:
  - `%d` = id

```json
{
  "success": true,
  "images": [
    "http://link.to/p1.jpg",
    "http://link.to/p2.jpg"
  ]
}
```
