"""Regression checks for blocked entrances, avenues, queues and grid rounding."""
import copy,json,unittest
from check_store_routes import audit
from validate_stores import ROOT,registry

class RouteRegression(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.stores=json.loads((ROOT/'Config/magazalar.json').read_text(encoding='utf-8'))['stores']
        cls.eq=registry()
    def test_start_inside_fixture(self):
        s=copy.deepcopy(self.stores[2]);s['points']['playerStart']['at']=s['fixtures'][0]['at']
        self.assertEqual(audit(s,self.eq)[0],['Player start blocked'])
    def test_blocked_cross_avenue(self):
        s=copy.deepcopy(self.stores[2]);s['fixtures'][0]['at']=[0,150,0]
        self.assertIn('Cross avenue obstructed',audit(s,self.eq)[0])
    def test_blocked_checkout_queue(self):
        s=copy.deepcopy(self.stores[2]);s['fixtures'][0]['at']=[-1300,-1000,0]
        self.assertTrue(any('Checkout queue obstructed' in e for e in audit(s,self.eq)[0]))
    def test_grid_rounding_does_not_hide_basket(self):
        s=copy.deepcopy(self.stores[0])
        basket=next(f for f in s['fixtures'] if f['equipment']=='basket_area');basket['at'][0]=330
        self.assertTrue(any('Unreachable sales face' in e and 'wall_shelf_2400' in e for e in audit(s,self.eq)[0]))
if __name__=='__main__':unittest.main()
