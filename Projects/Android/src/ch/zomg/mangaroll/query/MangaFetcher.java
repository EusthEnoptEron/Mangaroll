package ch.zomg.mangaroll.query;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by Simon on 2016/03/30.
 */
public class MangaFetcher extends Fetcher {

    private Descriptor descriptor;

    public MangaFetcher(Descriptor descriptor) {
        this.descriptor = descriptor;
    }

    public String[] fetch() {
        try {
            Document doc = Jsoup.connect(descriptor.getUrl()).get();
            List<String> links = new ArrayList<>();

            for(Element img : doc.select(descriptor.getItemSelector())) {
                String result = img.attr("src");
                if(result.isEmpty()) {
                    result = img.attr("href");
                }
                if(!result.isEmpty()) {
                    links.add(result);
                }
            }


            if(descriptor.getNextPageSelector() != null) {
                Elements nextPageLink = doc.select(descriptor.getNextPageSelector());
                setHasMore(nextPageLink.size() > 0);
                if(hasMore()) {
                    descriptor.setUrl(nextPageLink.get(0).attr("href"));
                }
            }

            return links.toArray(new String[links.size()]);
        } catch (IOException e) {
            return new String[0];
        }
    }
}
