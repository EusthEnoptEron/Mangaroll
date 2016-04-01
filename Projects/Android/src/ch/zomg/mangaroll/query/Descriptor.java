package ch.zomg.mangaroll.query;

import com.google.gson.annotations.SerializedName;

/**
 * Created by Simon on 2016/03/30.
 */
public class Descriptor implements Cloneable {
    enum Type {
        @SerializedName("container")
        CONTAINER,
        @SerializedName("manga")
        MANGA
    }

    private Type type;
    private String url;
    private String itemSelector;
    private String nameSelector;
    private String linkSelector;
    private String nextPageSelector;
    private Descriptor handler;

    public Descriptor() {

    }

    public Descriptor.Type getType() {
        return type;
    }

    public void setType(Descriptor.Type type) {
        this.type = type;
    }

    public Descriptor(String url) {
        this.url = url;

    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public String getItemSelector() {
        return itemSelector;
    }

    public void setItemSelector(String itemSelector) {
        this.itemSelector = itemSelector;
    }

    public String getNameSelector() {
        return nameSelector;
    }

    public void setNameSelector(String nameSelector) {
        this.nameSelector = nameSelector;
    }

    public String getLinkSelector() {
        return linkSelector;
    }

    public void setLinkSelector(String linkSelector) {
        this.linkSelector = linkSelector;
    }

    public String getNextPageSelector() {
        return nextPageSelector;
    }

    public void setNextPageSelector(String nextPageSelector) {
        this.nextPageSelector = nextPageSelector;
    }

    public Descriptor getHandler() {
        return handler;
    }

    public void setHandler(Descriptor handler) {
        this.handler = handler;
    }

    @Override
    public Descriptor clone() {
        Descriptor clone = new Descriptor();
        clone.url = url;
        clone.itemSelector = itemSelector;
        clone.nameSelector = nameSelector;
        clone.linkSelector = linkSelector;
        clone.nextPageSelector = nextPageSelector;
        clone.handler = handler;
        clone.type = type;
        return clone;
    }
}
