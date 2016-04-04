# Mangaroll

Mangaroll is a Gear VR app that lets you read your comics and manga by turning *yourself* around in a circle, preferably while sitting on an office chair to prevent bumping into walls. Through a more-or-less simple configuration file, you can also add online services to feed you with a (nearly) endless number of manga.

<a href="https://www.youtube.com/watch?v=t6HZ2MknAIY">![](http://www.zomg.ch/mangaroll.jpg)</a>

## Local manga

Just create a folder called either "Manga" or "Comics" in the SD card root and drop your image folders, zip (CBZ) or rar (CBR) files into it. They should then appear in the menu of the app.

## Remote manga

It's possible to fetch manga/comics from online sources. In order to get this to work, you will need to create a file called `services.json`.

Currently, you can place a file called `services.json` in your "Manga" folder that contains a list of remote services that may provide manga or comics.

The file is a simple array of objects, and there are two ways of defining a service: either by providing a pair of URLs to a compliant web server, or by describing how to directly fetch the images through CSS selectors (see [Service Configs](https://github.com/EusthEnoptEron/Mangaroll/wiki/Service-Configs)).

The contents of the file may e.g. look like so:

```javascript
[
  { // Example of using a web API
    "name" : "mangareader.net",
    "browseUrl" : "http://192.168.1.39:3000/mr/browse/{page}?id={id}",
    "showUrl"   : "http://192.168.1.39:3000/mr/show/{id}"
  },
  { // Example of using CSS selectors
    "name": "mangareader.net (dynamic)",
    "dynamic": {
      "url": "http://www.mangareader.net/popular", // Entry URL. Mandatory.
      "type": "container", // Either "container" or "manga". Mandatory. 
                           // In this case, this is a container that contains a list of manga titles
      "itemSelector": ".mangaresultitem", // 1st lvl selector to determine the child items. Mandatory.
      "nameSelector": ".manga_name h3", // 2nd lvl selector to determine the name of a child item. Mandatory.
      "linkSelector": ".manga_name h3 a", // 2nd lvl selector to determine the link to the item. Mandatory.
      "thumbSelector": ".imgsearchresults", // 2nd lvl selector to a link of a thumbnail
      "nextPageSelector": "#sp strong + a", // 1st lvl selector that should return the link to the next page or nothing
      "handler": {
        "type": "container", // Cpntainer that contains a list of chapters
        "itemSelector": "#listing tr",
        "linkSelector": "a",
        "nameSelector": "a",
        "handler": {
          "type": "manga", // Declares a manga (list of images)
          "itemSelector": "#imgholder img", // Selects the image to show
          "nextPageSelector": "#navi .next a" // Select the next page to fetch
        }
      }
    }
  }
]
```

### Web Service

When defining a web service, you will need two URLs that return either categories (containers) or manga/comics. [An example for a server can be found here.](https://gist.github.com/EusthEnoptEron/4e032a5bd8ee4049654b1d828816d9e1) The format is as follows.

#### browseUrl

**Required parameters in URL string**:
  - `{page}`
  - `{id}`

**Expected response**:
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


#### showUrl

**Required parameters in URL string**:
  - `{id}`

**Expected response**:
```json
{
  "success": true,
  "images": [
    "http://link.to/p1.jpg",
    "http://link.to/p2.jpg"
  ]
}
```

### Selectors

In order to turn a HTML element into an URL, the code will operate as follows:

1. Try to access the `href` attribute
2. Try to access the `src` attribute
3. Try to parse the `style` attribute for `url(...)`
