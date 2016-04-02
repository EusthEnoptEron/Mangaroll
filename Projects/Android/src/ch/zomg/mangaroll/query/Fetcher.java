package ch.zomg.mangaroll.query;

import android.util.Log;

import com.google.gson.Gson;

import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.net.URI;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

/**
 * Created by Simon on 2016/03/30.
 */
public abstract class Fetcher {
    private boolean hasMore = true;
    private String thumb = "";
    private String name = "";
    private static Pattern PATTERN_CSS_URL = Pattern.compile("url\\('?(.+?)'?\\)");

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

    public boolean hasMore() {
        return hasMore;
    }

    public abstract boolean isContainerProvider();

    protected void setHasMore(boolean hasMore) {
        this.hasMore = hasMore;
    }

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

    protected String extractURL(URI base, Elements element) {
        return element.size() > 0 ? extractURL(base, element.get(0)) : "";

    }
    protected String extractURL(URI base, Element element) {
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
