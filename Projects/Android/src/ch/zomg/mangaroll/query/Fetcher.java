package ch.zomg.mangaroll.query;

import android.util.Log;

import com.google.gson.Gson;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

import ch.zomg.mangaroll.MainActivity;

/**
 * Created by Simon on 2016/03/30.
 */
public abstract class Fetcher {
    private String thumb = "";
    private String name = "";
    private static Pattern PATTERN_CSS_URL = Pattern.compile("url\\('?(.+?)'?\\)");
    private boolean hasMorePages = true;
    private Queue<Element> currentElements = new LinkedList<>();
    protected URI base;
    private static String TAG = Fetcher.class.getName();

    protected Descriptor descriptor;

    protected Fetcher(Descriptor descriptor) {
        this.descriptor = descriptor;
        base = URI.create(descriptor.getUrl());

        Log.i(TAG, "Fetcher: " + descriptor.getUrl() + ", Fetch Size: " + descriptor.getFetchSize());
        hasMorePages = descriptor != null && isDescriptorValid();
    }

    protected abstract boolean isDescriptorValid();

    public boolean hasMore() {
        return hasMorePages || hasElements();
    }
    private boolean hasElements() {
        return currentElements != null && !currentElements.isEmpty();
    }

    public Element[] next() {
        if(!hasMorePages && !hasElements()) {
            Log.e(TAG, "Don't fetch when there are no further pages!");
            return new Element[0];
        }
        if(!hasElements()) {
            load();
        }

        List<Element> elements = new ArrayList<>();
        for(int i = 0; i < descriptor.getFetchSize() && !currentElements.isEmpty(); i++) {
            elements.add(currentElements.remove());
        }

        return elements.toArray(new Element[elements.size()]);
    }

    private void load() {
        hasMorePages = false;

        if(descriptor.getUrl() == null) {
            return;
        }

        try {
            Log.i(TAG, "Fetch new document from " + descriptor.getUrl());

            // Get items
            Document doc = Jsoup.connect(descriptor.getUrl()).userAgent(MainActivity.USER_AGENT).get();
            currentElements = new LinkedList(Arrays.asList(doc.select(descriptor.getItemSelector()).toArray(new Element[0])));

            Log.i(TAG, "Loaded " + currentElements.size() + " elements");

            // Get next page
            if(descriptor.getNextPageSelector() != null) {
                Elements nextPageLinks = doc.select(descriptor.getNextPageSelector());
                if (nextPageLinks.size() > 0 && currentElements.size() > 0) {
                    Element nextPageLink = nextPageLinks.get(0);
                    String link = nextPageLink.attr("href");
                    if (!link.isEmpty()) {
                        descriptor.setUrl(base.resolve(link).toString());
                        hasMorePages = true;
                    }
                }
            }

        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
        }

    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getThumb() {
        return thumb;
    }

    public void setThumb(String thumb) {
        this.thumb = thumb;
    }

    public abstract boolean isContainerProvider();

    public String getID() {
        return name == null ? "" : name;
    }

    public static Fetcher getInitialFetcher(String descriptorString) {
        Gson gson = new Gson();
        Descriptor descriptor = gson.fromJson(descriptorString, Descriptor.class);
        if (descriptor != null) {
            if (descriptor.getType() == Descriptor.Type.CONTAINER) {
                return new ContainerFetcher(descriptor);
            } else if(descriptor.getType() == Descriptor.Type.MANGA) {
                return new MangaFetcher(descriptor);
            }
        }
        return null;
    }

    protected String extractURL(Elements element) {
        return element.size() > 0 ? extractURL(element.get(0)) : "";

    }
    protected String extractURL(Element element) {
        String result = element.attr("href");
        if(result.isEmpty()) {
            result = element.attr("src");
        }

        if(result.isEmpty()) {
            Matcher matcher = PATTERN_CSS_URL.matcher(element.attr("style"));
            if(matcher.find()) {
                result = matcher.group(1).trim();
            }
        }
        if(!result.isEmpty()) {
            return base.resolve(result).toString();
        }

        return result;
    }

    protected String extractName(Elements elements) {
        String result = "";
        if(elements.size() > 0) {
            result = elements.get(0).text().trim();
        }
        return result;
    }
}
