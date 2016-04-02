# Mangaroll

Mangaroll is a Gear VR app that lets you read your comics and manga by turning *yourself* around in a circle, preferably while sitting on an office chair to prevent bumping into walls. Through a more-or-less simple configuration file, you can also add online services to feed you with a (nearly) endless number of manga.

![](http://www.zomg.ch/mangaroll.jpg)

## Services

It's possible to integrate simple online services that will serve manga. To get this to work, the app needs two links: one to browse manga, and one to view a certain one.

### Services.json

In all directories that are scanned for manga (currently the "Manga" directory) you can place a services.json that contains a list of remote services that may provide manga or comics.

The file is a simple array of objects, and there are two ways of defining a service: by providing a pair of URLs to an API-compliant web server, or by describing how to directly fetch the images through CSS selectors.

```javascript
[
	{
		"name" : "mangareader.net",
		"browseUrl" : "http://192.168.1.39:3000/mr/browse/{page}?id={id}",
		"showUrl"   : "http://192.168.1.39:3000/mr/show/{id}"
	},
{
		"name": "Mangareader (dynamic)",
	    "dynamic": {
	        "url": "http://www.mangareader.net/popular", // Entry URL. Mandatory.
	        "type": "container", // Either "container" or "manga". Mandatory.
	        "itemSelector": ".mangaresultitem", // 1st lvl selector to determine the child items. Mandatory.
	        "nameSelector": ".manga_name h3", // 2nd lvl selector to determine the name of a child item. Mandatory.
	        "linkSelector": ".manga_name h3 a", // 2nd lvl selector to determine the link to the item. Mandatory.
	        "thumbSelector": ".imgsearchresults", // 2nd lvl selector to a link of a thumbnail
	        "nextPageSelector": "#sp strong + a", // 1st lvl selector that should return the link to the next page or nothing
	        "handler": {
	            "type": "container",
	            "itemSelector": "#listing tr",
	            "linkSelector": "a",
	            "nameSelector": "a",
	            "handler": {
	                "type": "manga",
	                "itemSelector": "#imgholder img",
	                "nextPageSelector": "#navi .next a"
	            }
	        }
	    }
 	 }
]

```

### Web Service

When defining a web service, you will need two URLs that return either categories (containers) or manga/comics. The format is as follows.

#### Browse view

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


#### Read view

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
