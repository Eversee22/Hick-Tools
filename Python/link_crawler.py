import re
import urllib.request
from urllib import robotparser
from urllib.parse import urljoin
from urllib.error import URLError, HTTPError, ContentTooShortError
from urllib.parse import urlparse
# from lxml.html import fromstring
# from chp1.throttle import Throttle
import time
import os


class Throttle:
    """ Add a delay between downloads to the same domain
    """
    def __init__(self, delay):
        # amount of delay between downloads for each domain
        self.delay = delay
        # timestamp of when a domain was last accessed
        self.domains = {}

    def wait(self, url):
        domain = urlparse(url).netloc
        last_accessed = self.domains.get(domain)

        if self.delay > 0 and last_accessed is not None:
            sleep_secs = self.delay - (time.time() - last_accessed)
            if sleep_secs > 0:
                # domain has been accessed recently
                # so need to sleep
                time.sleep(sleep_secs)
        # update the last accessed time
        self.domains[domain] = time.time()


def download(url, num_retries=2, user_agent='wswp', charset='utf-8', proxy=None):
    """ Download a given URL and return the page content
        args:
            url (str): URL
        kwargs:
            user_agent (str): user agent (default: wswp)
            charset (str): charset if website does not include one in headers
            proxy (str): proxy url, ex 'http://IP' (default: None)
            num_retries (int): number of retries if a 5xx error is seen (default: 2)
    """
    print('Downloading:', url)
    request = urllib.request.Request(url)
    request.add_header('User-agent', user_agent)
    try:
        if proxy:
            proxy_support = urllib.request.ProxyHandler({'http': proxy,'https': proxy})
            opener = urllib.request.build_opener(proxy_support)
            urllib.request.install_opener(opener)
        resp = urllib.request.urlopen(request)
        cs = resp.headers.get_content_charset()
        if not cs:
            cs = charset
        htmlbytes = resp.read()
        html = htmlbytes.decode(cs)
    except (URLError, HTTPError, ContentTooShortError) as e:
        print('Download error:', e)
        html = None
        if num_retries > 0:
            if hasattr(e, 'code') and 500 <= e.code < 600:
                # recursively retry 5xx HTTP errors
                return download(url, num_retries - 1)
    except UnicodeDecodeError as e:
        html = None
        print('- Decode error: ', e)
        name = urllib.parse.urlparse(url)
        name = os.path.basename(name.path)
        print(name)
        with open(name, 'wb') as f:
            f.write(htmlbytes)
    except Exception as e:
        html = None
        print('- Error: ', e)
    return html


def get_robots_parser(robots_url):
    " Return the robots parser object using the robots_url "
    rp = robotparser.RobotFileParser()
    rp.set_url(robots_url)
    rp.read()
    return rp


def get_links(html):
    " Return a list of links (using simple regex matching) from the html content "
    # a regular expression to extract all links from the webpage
    webpage_regex = re.compile("""<a[^>]+href=["'](.*?)["']""", re.IGNORECASE)
    # list of all links from the webpage
    return webpage_regex.findall(html)


def scrape_callback(url, html):
    """ Scrape each row from the country or district data using XPath and lxml """
    fields = ('area', 'population', 'iso', 'country_or_district', 'capital',
              'continent', 'tld', 'currency_code', 'currency_name',
              'phone', 'postal_code_format', 'postal_code_regex',
              'languages', 'neighbours')
    if re.search('/view/', url):
        tree = fromstring(html)
        all_rows = [
            tree.xpath('//tr[@id="places_%s__row"]/td[@class="w2p_fw"]' % field)[0].text_content()
            for field in fields]
        print(url, all_rows)


def link_crawler(start_url, link_regex, robots_url=None, user_agent='wswp',
                 proxy=None, delay=1, max_depth=4, scrape_callback=None):
    """ Crawl from the given start URL following links matched by link_regex. In the current
        implementation, we do not actually scrapy any information.

        args:
            start_url (str): web site to start crawl
            link_regex (str): regex to match for links
        kwargs:
            robots_url (str): url of the site's robots.txt (default: start_url + /robots.txt)
            user_agent (str): user agent (default: wswp)
            proxy (str): proxy url, ex 'http://IP' (default: None)
            delay (int): seconds to throttle between requests to one domain (default: 3)
            max_depth (int): maximum crawl depth (to avoid traps) (default: 4)
            scrape_callback (function): function to call after each download (default: None)
    """
    crawl_queue = [start_url]
    # keep track which URL's have seen before
    seen = {}
    data = []
    # if not robots_url:
    #     robots_url = '{}/robots.txt'.format(start_url)
    # rp = get_robots_parser(robots_url)
    throttle = Throttle(delay)
    while crawl_queue:
        url = crawl_queue.pop()
        # check url passes robots.txt restrictions
        if True: #rp.can_fetch(user_agent, url):
            depth = seen.get(url, 0)
            if depth == max_depth:
                print('Skipping %s due to depth' % url)
                continue
            throttle.wait(url)
            # time.sleep(1)
            html = download(url, user_agent=user_agent, proxy=proxy)
            if not html:
                continue
            if scrape_callback:
                data.extend(scrape_callback(url, html) or [])
            # filter for links matching our regular expression
            for link in get_links(html):
                # print(link)
                if re.search(link_regex, link):
                    abs_link = urljoin(url, link)
                    # print(abs_link)
                    if abs_link not in seen:
                        seen[abs_link] = depth + 1
                        crawl_queue.append(abs_link)
        else:
            print('Blocked by robots.txt:', url)


if __name__ == '__main__':
    import sys
    if len(sys.argv) == 1:
        print('URL needed !!!')
        sys.exit(1)
    url = sys.argv[1]
    link_crawler(url,'.*',max_depth=3,proxy=None)
